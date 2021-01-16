(define (problem ijrr_2)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street16 street42 - street
		building7 building18 building28 building35 - building
	)
	(:init
		(atLocation survivor0 street16)
		(atLocation survivor1 building7)
		(onFire building28)
		(onFire building35)
		(underRubble street42)
		(underRubble building18)
		(needsRepair building18)
		(needsRepair building28)
		(needsRepair building35)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street42))
		(not (needsRepair building18))
		(not (needsRepair building28))
		(not (needsRepair building35))
	))
	(:metric minimize total-time)
)
