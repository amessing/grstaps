(define (problem ijrr_2)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street37 street48 - street
		building0 building20 building25 building28 - building
	)
	(:init
		(atLocation survivor0 street48)
		(atLocation survivor1 building28)
		(onFire building0)
		(onFire building25)
		(underRubble street37)
		(underRubble building20)
		(needsRepair building0)
		(needsRepair building20)
		(needsRepair building25)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street37))
		(not (needsRepair building0))
		(not (needsRepair building20))
		(not (needsRepair building25))
	))
	(:metric minimize total-time)
)
