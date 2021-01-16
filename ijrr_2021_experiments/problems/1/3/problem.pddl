(define (problem ijrr_3)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street0 street6 - street
		building14 building17 building20 building22 - building
	)
	(:init
		(atLocation survivor0 street6)
		(atLocation survivor1 building14)
		(onFire building20)
		(onFire building22)
		(underRubble street0)
		(underRubble building17)
		(needsRepair building17)
		(needsRepair building20)
		(needsRepair building22)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street0))
		(not (needsRepair building17))
		(not (needsRepair building20))
		(not (needsRepair building22))
	))
	(:metric minimize total-time)
)
